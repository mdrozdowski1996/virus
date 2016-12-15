#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <vector>
#include <queue>
#include <map>
#include <set>
#include <memory>
#include <algorithm>

class VirusNotFound : public std::exception {
	virtual const char *what() const throw() {
		return "VirusNotFound";
	}
};

class VirusAlreadyCreated : public std::exception {
	virtual const char *what() const throw() {
		return "VirusAlreadyCreated";
	}
};

class TriedToRemoveStemVirus : public std::exception {
	virtual const char *what() const throw() {
		return "TriedToRemoveStemVirus";
	}
};

template<class Virus>
class VirusGenealogy {
public:
	typedef typename Virus::id_type id_type;

	VirusGenealogy() = delete;

	VirusGenealogy(const VirusGenealogy &) = delete;

	VirusGenealogy &operator=(const VirusGenealogy &) = delete;

	VirusGenealogy(const id_type& stem_id) : stem_id(stem_id) {
		viruses[stem_id] = std::make_shared<VirusNode>(stem_id);
	}

	id_type get_stem_id() const noexcept {
		return stem_id;
	}

	std::vector<id_type> get_children(const id_type& id) const {
		if (!exists(id)) {
			throw VirusNotFound();
		}

		auto children = viruses.find(id)->second->children;
		std::vector<id_type> children_vector;

		for (auto child : children) {
			children_vector.push_back(child.get()->id);
		}

		return children_vector;
	}

	std::vector<id_type> get_parents(id_type const &id) const {
		if (!exists(id)) {
			throw VirusNotFound();
		}

		auto parents = viruses.find(id)->second->parents;
		std::vector<id_type> parents_vector;

		for (auto parent : parents) {
			parents_vector.push_back(parent.get()->id);
		}

		return parents_vector;
	}

	bool exists(const id_type& id) const noexcept {
		return viruses.find(id) != viruses.end();
	}

	const Virus &operator[](const id_type& id) const {
		if (!exists(id)) {
			throw VirusNotFound();
		}

		auto virus_node = viruses.find(id)->second;
		//auto &virus_ref = *(virus.get());
		return virus_node.get()->virus;
	}

	void create(const id_type& id, const id_type& parent_id) {
		if (exists(id)) {
			throw VirusAlreadyCreated();
		}

		if (!exists(parent_id)) {
			throw VirusNotFound();
		}

		auto new_virus_ptr = std::make_shared<VirusNode>(id);
		auto parent_node_ptr = viruses.find(parent_id)->second;
		new_virus_ptr->add_parent(parent_node_ptr);
		parent_node_ptr->add_child(new_virus_ptr);
		viruses[id] = new_virus_ptr;
	}

	void create(const id_type& id, const std::vector<id_type>& parent_ids) {
		if (exists(id)) {
			throw VirusAlreadyCreated();
		}

		if (parent_ids.empty()) {
			throw VirusNotFound();
		} else {
			for (auto parent_id: parent_ids) {
				if (!exists(parent_id)) {
					throw VirusNotFound();
				}
			}
		}

		auto new_virus_ptr = std::make_shared<VirusNode>(id);
		for (auto parent_id : parent_ids) {
			auto parent_node_ptr = viruses.find(parent_id)->second;
			new_virus_ptr->add_parent(parent_node_ptr);
			parent_node_ptr->add_child(new_virus_ptr);
		}

		viruses[id] = new_virus_ptr;
	}

	void connect(const id_type& child_id, const id_type& parent_id) {
		if (!exists(parent_id) || !exists(child_id)) {
			throw VirusNotFound();
		}

		auto parent_node_ptr = viruses.find(parent_id)->second;
		auto child_node_ptr = viruses.find(child_id)->second;
		child_node_ptr->add_parent(parent_node_ptr);
		parent_node_ptr->add_child(child_node_ptr);
	}

	void remove(const id_type& id) {
		if (!exists(id)) {
			throw VirusNotFound();
		}

		if (id == stem_id) {
			throw TriedToRemoveStemVirus();
		}
		
		std::queue<std::shared_ptr<VirusNode>> to_remove;
		std::map<id_type, std::shared_ptr<VirusNode>> viruses_copy(viruses);
		
		auto current_virus = viruses_copy.find(id);
		to_remove.push(current_virus -> second);
		
		while(!to_remove.empty()){
			auto rem_it = to_remove.front();
			to_remove.pop();
			
			auto new_id = rem_it->id;
			
			for (auto parent : rem_it->parents) {
				parent->children.erase(rem_it);
			}
			
			for (auto child : rem_it->childrens){
				child->parents.erase(rem_it);
				if( child->parents.size() == static_cast<size_t>(0)
					&& child->id != stem_id) {
						to_remove.push(child);
				}
			}
			
			viruses_copy.erase(new_id);
		}
		
		viruses.swap(viruses_copy);
	}

private:
	class VirusNode {
	public:
		id_type id;
		Virus virus;
		std::set<std::shared_ptr<VirusNode>> children;
		std::set<std::shared_ptr<VirusNode>> parents;

		VirusNode(id_type _id) : id(_id), virus(Virus(_id)) {};

		void add_child(std::shared_ptr<VirusNode> virus_node) {
			children.insert(virus_node);
		}

		void remove_child(std::shared_ptr<VirusNode> virus_node) {
			children.erase(virus_node);
		}

		void add_parent(std::shared_ptr<VirusNode> virus_node) {
			parents.insert(virus_node);
		}

		void remove_parent(std::shared_ptr<VirusNode> virus_node) {
			parents.erase(virus_node);
		}

		void add_parents(std::vector<std::shared_ptr<VirusNode>> virus_nodes) {
			parents.insert(virus_nodes.begin(), virus_nodes.end());
		}
	};

	std::map<id_type, std::shared_ptr<VirusNode>> viruses;

	const id_type stem_id;
};

#endif 
