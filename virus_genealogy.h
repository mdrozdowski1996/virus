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
	VirusGenealogy(typename Virus::id_type const &stem_id);

	VirusGenealogy() = delete;

	VirusGenealogy(const VirusGenealogy &) = delete;

	VirusGenealogy &operator=(const VirusGenealogy &) = delete;

	typename Virus::id_type get_stem_id() const noexcept;

	std::vector<typename Virus::id_type>
	get_children(typename Virus::id_type const &id) const;

	std::vector<typename Virus::id_type>
	get_parents(typename Virus::id_type const &id) const;

	bool exists(typename Virus::id_type const &id) const noexcept;

	const Virus &operator[](typename Virus::id_type const &id) const;

	void create(typename Virus::id_type const &id,
				typename Virus::id_type const &parent_id);

	void create(typename Virus::id_type const &id,
				std::vector<typename Virus::id_type> const &parent_ids);

	void connect(typename Virus::id_type const &child_id,
				 typename Virus::id_type const &parent_id);

	void remove(typename Virus::id_type const &id);

private:
	class VirusNode{
	public:
		typename Virus::id_type id;
		Virus virus;
		std::set<std::shared_ptr<VirusNode>> children;
		std::set<std::shared_ptr<VirusNode>> parents;

		VirusNode(typename Virus::id_type _id) : id(_id), virus(Virus(_id)) {};

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

	std::map<typename Virus::id_type, std::shared_ptr<VirusNode>> viruses;

	const typename Virus::id_type stem_id;
};

// Tworzy nową genealogię.
// Tworzy także węzeł wirusa macierzystego o identyfikatorze stem_id.
template<class Virus>
VirusGenealogy<Virus>::VirusGenealogy(typename Virus::id_type const &stem_id):
		stem_id(stem_id) {
	viruses[stem_id] = std::make_shared<VirusNode>(stem_id);
}

// Zwraca identyfikator wirusa macierzystego.
template<class Virus>
typename Virus::id_type VirusGenealogy<Virus>::get_stem_id() const noexcept {
	return stem_id;
}

// Zwraca listę identyfikatorów bezpośrednich następników wirusa
// o podanym identyfikatorze.   
// Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
template<class Virus>
std::vector<typename Virus::id_type> VirusGenealogy<Virus>::get_children(
		typename Virus::id_type const &id) const {
	if (!exists(id)) {
		throw VirusNotFound();
	}

	auto children = viruses.find(id)->second->children;
	std::vector<typename Virus::id_type> children_vector;

	for (auto child : children) {
		children_vector.push_back(child.get()->id);
	}

	return children_vector;
}

// Zwraca listę identyfikatorów bezpośrednich poprzedników wirusa
// o podanym identyfikatorze.
// Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
template<class Virus>
std::vector<typename Virus::id_type> VirusGenealogy<Virus>::get_parents(
		typename Virus::id_type const &id) const {
	if (!exists(id)) {
		throw VirusNotFound();
	}

	auto parents = viruses.find(id)->second->parents;
	std::vector<typename Virus::id_type> parents_vector;

	for (auto parent : parents) {
		parents_vector.push_back(parent.get()->id);
	}

	return parents_vector;
}

// Sprawdza, czy wirus o podanym identyfikatorze istnieje.
template<class Virus>
bool VirusGenealogy<Virus>::exists(
		typename Virus::id_type const &id) const noexcept {
	return viruses.find(id) != viruses.end();
}

// Zwraca referencję do obiektu reprezentującego wirus o podanym
// identyfikatorze.
// Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
template<class Virus>
const Virus &
VirusGenealogy<Virus>::operator[](typename Virus::id_type const &id) const {
	if (!exists(id)) {
		throw VirusNotFound();
	}

	auto virus_node = viruses.find(id)->second;
//    auto &virus_ref = *(virus.get());
	return virus_node.get()->virus;
}

// Tworzy węzeł reprezentujący nowy wirus o identyfikatorze id
// powstały z wirusa o podanym identyfikatorze parent_id.
// Zgłasza wyjątek VirusAlreadyCreated, jeśli wirus o identyfikatorze
// id już istnieje.
// Zgłasza wyjątek VirusNotFound, jeśli poprzednik nie istnieje.
template<class Virus>
void VirusGenealogy<Virus>::create(typename Virus::id_type const &id,
								   typename Virus::id_type const &parent_id) {
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

// Tworzy węzeł reprezentujący nowy wirus o identyfikatorze id
// powstały z wirusów o podanych identyfikatorach parent_ids.
// Zgłasza wyjątek VirusAlreadyCreated, jeśli wirus o identyfikatorze
// id już istnieje.
// Zgłasza wyjątek VirusNotFound, jeśli któryś z wyspecyfikowanych
// poprzedników nie istnieje.
template<class Virus>
void VirusGenealogy<Virus>::create(typename Virus::id_type const &id,
								   std::vector<typename Virus::id_type> const &parent_ids) {
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

// Dodaje nową krawędź w grafie genealogii.
// Zgłasza wyjątek VirusNotFound, jeśli któryś z podanych wirusów nie istnieje.
template<class Virus>
void VirusGenealogy<Virus>::connect(typename Virus::id_type const &child_id,
									typename Virus::id_type const &parent_id) {
	if (!exists(parent_id) || !exists(child_id)) {
		throw VirusNotFound();
	}

	auto parent_node_ptr = viruses.find(parent_id)->second;
	auto child_node_ptr = viruses.find(child_id)->second;
	child_node_ptr->add_parent(parent_node_ptr);
	parent_node_ptr->add_child(child_node_ptr);
}

// Usuwa wirus o podanym identyfikatorze.
// Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
// Zgłasza wyjątek TriedToRemoveStemVirus przy próbie usunięcia
// wirusa macierzystego.
template<class Virus>
void VirusGenealogy<Virus>::remove(typename Virus::id_type const &id) {
	if (!exists(id)) {
		throw VirusNotFound();
	}

	if (id == stem_id) {
		throw TriedToRemoveStemVirus();
	}
	
	std::queue < std::shared_ptr < VirusNode > > to_remove;
	std::map<typename Virus::id_type, std::shared_ptr<VirusNode>> viruses_copy(viruses);
	
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
	
//Zakładamy, że:
//* klasa Virus ma konstruktor przyjmujący argument typu Virus::id_type;
//* klasa Virus ma metodę Virus::id_type get_id() const;
//* typ Virus::id_type ma konstruktor bezargumentowy, konstruktor
//        kopiujący, operator przypisania;
//* wartości typu Virus::id_type tworzą porządek liniowy i można je
//        porównywać za pomocą operatorów ==, !=, <=, >=, <, >.
//
//Ponadto:
//* wszystkie metody klasy VirusGenealogy powinny gwarantować silną odporność
//na wyjątki, a tam, gdzie to jest możliwe i pożądane, powinny być no-throw;
//* próba użycia konstruktora kopiującego lub operatora przypisania dla
//obiektów klasy VirusGenealogy powinna zakończyć się błędem kompilacji;
//* zachowanie obiektu typu VirusGenealogy po utworzeniu cyklu pozostaje
//niezdefiniowane -- nie trzeba wykrywać takiej sytuacji;
//* wyjątki VirusAlreadyCreated, VirusNotFound oraz TriedToRemoveStemVirus
//powinny być zdefiniowane poza klasą VirusGenealogy i powinny dziedziczyć
//        z std::exception;
//* wyszukiwanie wirusów powinno być szybsze niż liniowe.
//
//Zarządzanie pamięcią powinno być zrealizowane za pomocą sprytnych wskaźników
//        z biblioteki standardowej.

#endif 
