#include "filter_table.h"
#include "scene.h"
#include "relation.h"
#include "filter.h"
#include "symtab.h"

using namespace std;

filter_table& get_filter_table()
{
    static filter_table inst;
    return inst;
}

// filters/node.cpp
filter_table_entry* node_filter_entry();
filter_table_entry* all_nodes_filter_entry();
filter_table_entry* remove_node_filter_entry();
filter_table_entry* node_position_filter_entry();
filter_table_entry* node_rotation_filter_entry();
filter_table_entry* node_scale_filter_entry();
filter_table_entry* node_bbox_filter_entry();

// filters/distance.cpp
filter_table_entry* distance_filter_entry();
filter_table_entry* distance_select_filter_entry();
filter_table_entry* closest_filter_entry();
filter_table_entry* farthest_filter_entry();

// filters/volume.cpp
filter_table_entry* volume_filter_entry();
filter_table_entry* volume_select_filter_entry();
filter_table_entry* largest_filter_entry();
filter_table_entry* smallest_filter_entry();
filter_table_entry* larger_filter_entry();
filter_table_entry* smaller_filter_entry();
filter_table_entry* larger_select_filter_entry();
filter_table_entry* smaller_select_filter_entry();

// filters/distance_on_axis.cpp
filter_table_entry* distance_on_axis_filter_entry();
filter_table_entry* distance_on_axis_filter_select_entry();

// filters/intersect.cpp
filter_table_entry* intersect_filter_entry();
filter_table_entry* intersect_select_filter_entry();

// filters/contain.cpp
filter_table_entry* contain_filter_entry();
filter_table_entry* contain_select_filter_entry();

// filters/tag_select.cpp
filter_table_entry* tag_select_filter_entry();

filter_table::filter_table()
{
  add(node_filter_entry());
  add(all_nodes_filter_entry());
  add(remove_node_filter_entry());
  add(node_position_filter_entry());
  add(node_rotation_filter_entry());
  add(node_scale_filter_entry());
  add(node_bbox_filter_entry());

  add(distance_filter_entry());
  add(distance_select_filter_entry());
  add(closest_filter_entry());
  add(farthest_filter_entry());

  add(volume_filter_entry());
  add(volume_select_filter_entry());
  add(largest_filter_entry());
  add(smallest_filter_entry());
  add(larger_filter_entry());
  add(smaller_filter_entry());
  add(larger_select_filter_entry());
  add(smaller_select_filter_entry());

  add(distance_on_axis_filter_entry());
  add(distance_on_axis_select_filter_entry());

  add(intersect_filter_entry());
  add(intersect_select_filter_entry());

  add(contain_filter_entry());
  add(contain_select_filter_entry());

  add(tag_select_filter_entry());
}

void filter_table::proxy_get_children(map<string, cliproxy*>& c)
{
    c["timers"] = &timers;
    
    map<string, filter_table_entry*>::iterator i, iend;
    for (i = t.begin(), iend = t.end(); i != iend; ++i)
    {
        c[i->first] = i->second;
    }
}

template <typename T>
class single_combination_generator
{
    public:
        single_combination_generator(const std::vector<T>& elems, int n, bool ordered, bool allow_repeat)
            : elems(elems), indices(n), nelems(elems.size()), n(n),
              ordered(ordered), allow_repeat(allow_repeat), finished(false)
        {
            assert(n <= nelems);
            reset();
        }
        
        void reset()
        {
            if (!ordered && !allow_repeat)
            {
                for (int i = 0; i < n; ++i)
                {
                    indices[i] = n - i - 1;
                }
            }
            else
            {
                fill(indices.begin(), indices.end(), 0);
            }
        }
        
        bool next(std::vector<T>& comb)
        {
            if (nelems == 0 || n == 0)
            {
                return false;
            }
            
            comb.resize(n);
            std::set<int> s;
            while (!finished)
            {
                bool has_repeat = false;
                s.clear();
                for (int i = 0; i < n; ++i)
                {
                    comb[i] = elems[indices[i]];
                    if (!has_repeat && !allow_repeat && ordered)
                    {
                        /*
                         incrementing technique guarantees no
                         repeats in the case ordered = false
                         and allow_repeats = false
                        */
                        std::pair<std::set<int>::iterator, bool> p = s.insert(indices[i]);
                        if (!p.second)
                        {
                            has_repeat = true;
                        }
                    }
                }
                increment(nelems - 1, 0);
                if (allow_repeat || !has_repeat)
                {
                    return true;
                }
            }
            return false;
        }
        
    private:
        int increment(int max, int i)
        {
            if (i == n - 1)
            {
                if (++indices[i] > max)
                {
                    finished = true;
                }
                return indices[i];
            }
            if (++indices[i] > max)
            {
                if (ordered)
                {
                    increment(max, i + 1);
                    indices[i] = 0;
                }
                else
                {
                    if (allow_repeat)
                    {
                        // maintain indices[i] >= indices[i+1]
                        indices[i] = increment(max, i + 1);
                    }
                    else
                    {
                        // maintain indices[i] > indices[i+1]
                        indices[i] = increment(max - 1, i + 1) + 1;
                    }
                }
            }
            return indices[i];
        }
        
        const std::vector<T>& elems;
        std::vector<int> indices;
        int n, nelems;
        bool ordered, allow_repeat, finished;
};

filter* filter_table::make_filter(const string& pred, Symbol* root, soar_interface* si, scene* scn, filter_input* input) const
{
    map<std::string, filter_table_entry*>::const_iterator i = t.find(pred);
    if (i == t.end() || i->second->create == NULL)
    {
        return NULL;
    }
    return (*(i->second->create))(root, si, scn, input);
}

void filter_table::get_all_atoms(scene* scn, vector<string>& atoms) const
{
    vector<const sgnode*> all_nodes;
    vector<string> all_node_names;
    scn->get_all_nodes(all_nodes);
    all_node_names.resize(all_nodes.size() - 1);
    for (int i = 1; i < all_nodes.size(); ++i)    // don't use world node
    {
        all_node_names[i - 1] = all_nodes[i]->get_name();
    }
    
    map<string, filter_table_entry*>::const_iterator i, iend;
    for (i = t.begin(), iend = t.end(); i != iend; ++i)
    {
        const filter_table_entry* e = i->second;
        if (e->calc != NULL)
        {
            vector<string> args;
            single_combination_generator<string> gen(all_node_names, e->parameters.size(), e->ordered, e->allow_repeat);
            while (gen.next(args))
            {
                stringstream ss;
                ss << e->name << "(";
                for (int j = 0; j < args.size() - 1; ++j)
                {
                    ss << args[j] << ",";
                }
                ss << args.back() << ")";
                atoms.push_back(ss.str());
                args.clear();
            }
        }
    }
}

void filter_table::get_predicates(vector<string>& preds) const
{
    map<string, filter_table_entry*>::const_iterator i, iend;
    for (i = t.begin(), iend = t.end(); i != iend; ++i)
    {
        preds.push_back(i->first);
    }
}

bool filter_table::get_params(const string& pred, vector<string>& p) const
{
    map<string, filter_table_entry*>::const_iterator i = t.find(pred);
    if (i == t.end())
    {
        return false;
    }
    p = i->second->parameters;
    return true;
}

void filter_table::add(filter_table_entry* e)
{
    assert(t.find(e->name) == t.end());
    t[e->name] = e;
}

void filter_table::update_relations(const scene* scn, const vector<int>& dirty, int time, relation_table& rt) const
{
    vector<const sgnode*> nodes;
    scn->get_all_nodes(nodes);
    nodes.erase(nodes.begin());
    
    map<string, filter_table_entry*>::const_iterator i, iend;
    for (i = t.begin(), iend = t.end(); i != iend; ++i)
    {
        const filter_table_entry* e = i->second;
        if (e->calc != NULL && nodes.size() >= e->parameters.size())
        {
            relation& r = rt[e->name];
            if (r.arity() == 0)
            {
                // +1 for the time argument
                r.reset(e->parameters.size() + 1);
            }
            vector<const sgnode*> args;
            vector<int> arg_ids;
            single_combination_generator<const sgnode*> gen(nodes, e->parameters.size(), e->ordered, e->allow_repeat);
            while (gen.next(args))
            {
                bool params_dirty = false;
                arg_ids.resize(args.size());
                for (int j = 0, jend = args.size(); j < jend; ++j)
                {
                    arg_ids[j] = args[j]->get_id();
                    if (has(dirty, args[j]->get_id()))
                    {
                        params_dirty = true;
                    }
                }
                if (params_dirty)
                {
                    timer& t = timers.get_or_add(i->first.c_str());
                    t.start();
                    bool pos = (*e->calc)(scn, args);
                    t.stop();
                    if (pos)
                    {
                        if (e->ordered)
                        {
                            r.add(time, arg_ids);
                        }
                        else
                        {
                            // true for all permutations
                            single_combination_generator<int> gen2(arg_ids, arg_ids.size(), true, e->allow_repeat);
                            int_tuple perm;
                            while (gen2.next(perm))
                            {
                                r.add(time, perm);
                            }
                        }
                    }
                }
                args.clear();
            }
        }
    }
}

filter_table_entry::filter_table_entry()
    : create(NULL), calc(NULL), ordered(false), allow_repeat(false)
{
    set_help("Reports information about this filter type.");
}

void filter_table_entry::proxy_use_sub(const vector<string>& args, ostream& os)
{
    os << "parameters:";
    for (int i = 0, iend = parameters.size(); i < iend; ++i)
    {
        os << " " << parameters[i];
    }
    os << endl;
    if (ordered)
    {
        os << "ordered ";
    }
    if (allow_repeat)
    {
        os << "repeat ";
    }
    if (calc)
    {
        os << "basic";
    }
    os << endl;
}
/*
Example input:

(<ot> ^type on-top ^a <ota> ^b <otb>)
(<ota> ^type node ^name box1)
(<otb> ^type node ^name box2)
*/
filter* parse_filter_spec(soar_interface* si, Symbol* root, scene* scn)
{
    wme_list children, params;
    wme_list::iterator i;
    string pname, ftype, itype;
    filter_input* input;
    bool fail;
    filter* f;
    
    if (!root->is_identifier())
    {
        string strval;
        long intval;
        double floatval;
        
        if (get_symbol_value(root, strval))
        {
            return new const_filter<string>(strval);
        }
        else if (get_symbol_value(root, intval))
        {
            return new const_filter<int>(intval);
        }
        else if (get_symbol_value(root, floatval))
        {
            return new const_filter<double>(floatval);
        }
        return NULL;
    }
    
    fail = false;
    si->get_child_wmes(root, children);
    for (i = children.begin(); i != children.end(); ++i)
    {
        if (!get_symbol_value(si->get_wme_attr(*i), pname))
        {
            continue;
        }
        Symbol* cval = si->get_wme_val(*i);
        if (pname == "type")
        {
            if (!get_symbol_value(cval, ftype))
            {
                return NULL;
            }
        }
        else if (pname == "input-type")
        {
            if (!get_symbol_value(cval, itype))
            {
                return NULL;
            }
        }
        else if (pname != "status" && pname != "result")
        {
            params.push_back(*i);
        }
    }
    
    // The combine type check is a bit of a hack
    if (itype == "concat" || ftype == "combine")
    {
        input = new concat_filter_input();
    }
    else if (params.size() == 0)
    {
        input = new null_filter_input();
    }
    else
    {
        input = new product_filter_input();
    }
    
    for (i = params.begin(); i != params.end(); ++i)
    {
        if (!get_symbol_value(si->get_wme_attr(*i), pname))
        {
            continue;
        }
        Symbol* cval = si->get_wme_val(*i);
        filter* cf = parse_filter_spec(si, cval, scn);
        if (!cf)
        {
            fail = true;
            break;
        }
        input->add_param(pname, cf);
    }
    
    if (!fail)
    {
        if (ftype == "combine")
        {
            //f = new passthru_filter(root, si, input);
        }
        else
        {
            f = get_filter_table().make_filter(ftype, root, si, scn, input);
        }
    }
    
    if (fail || ftype == "" || f == NULL)
    {
        delete input;
        return NULL;
    }
    return f;
}
