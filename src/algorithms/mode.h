#ifndef MODE_H
#define MODE_H

#include <set>
#include <vector>
#include <utility>
#include "mat.h"
#include "relation.h"
#include "scene_sig.h"
#include "foil.h"
#include "model.h"

class em_train_data;
class sig_info;
class classifier;

class em_mode : public serializable {
public:
	em_mode(bool noise, bool manual, const model_train_data &data);
	~em_mode();
	
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
	bool cli_inspect(int first, const std::vector<std::string> &args, std::ostream &os);
	void add_example(int i, const std::vector<int> &obj_map);
	void del_example(int i);
	void predict(const scene_sig &s, const rvec &x, const std::vector<int> &obj_map, rvec &y) const;
	void largest_const_subset(std::vector<int> &subset);
	const std::set<int> &get_noise(int sigindex) const;
	void get_noise_sigs(std::vector<int> &sigs);
	double calc_prob(int target, const scene_sig &sig, const rvec &x, double y, std::vector<int> &best_assign, double &best_error) const;
	bool update_fits();
	void set_linear_params(const scene_sig &dsig, int target, const mat &coefs, const rvec &inter);
	bool uniform_sig(int sig, int target) const;
	void learn_obj_clauses(const relation_table &rels);
	void get_members(std::vector<int> &m) const;

	int size() const { return members.size(); }
	bool is_new_fit() const { return new_fit; }
	bool is_manual() const { return manual; }
	void reset_new_fit() { new_fit = false; }
	
	const scene_sig &get_sig() const { return sig; }
	const relation &get_member_rel() const { return member_rel; }

	bool map_objs(int target, const scene_sig &dsig, const relation_table &rels, std::vector<int> &mapping) const;
	
	int get_num_nonzero_coefs() const;
	
	/*
	 Each pair of modes has one classifier associated with it. For
	 mode i, the classifier for it and mode j is stored in the
	 j_th element of this vector. Elements 0 - i of this vector
	 are NULL since those classifiers are already present in a
	 previous mode's classifier vector.
	*/
	std::vector<classifier*> classifiers;
	
	bool classifier_stale;
	
private:
	class mem_info : public serializable {
	public:
		std::vector<int> obj_map; // mapping from variable in mode sig -> object index in instance
		
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);
	};
	
	std::map<int, mem_info> members;
	
	bool stale, noise, new_fit, manual;
	const model_train_data &data;
	
	mat lin_coefs;
	rvec lin_inter;
	int n_nonzero;
	relation member_rel;
	scene_sig sig;
	std::vector<std::vector<int> > obj_maps; 
	
	/*
	 Noise data sorted by their Y values. First element in pair is the Y value,
	 second is the index.
	*/
	std::set<std::pair<double, int> > sorted_ys;
	
	/*
	 Each object the model is conditioned on needs to be
	 identifiable with a set of first-order Horn clauses
	 learned with FOIL.
	*/
	std::vector<std::vector<clause> > obj_clauses;
};

#endif
