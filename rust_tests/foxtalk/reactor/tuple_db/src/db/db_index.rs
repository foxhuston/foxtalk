use std::collections::{HashMap, HashSet};
use std::hash::Hash;

#[repr(transparent)]
pub struct DbIndex<K, V> { map: HashMap<K, HashSet<V>> }

impl<K, V> DbIndex<K, V>
where K: PartialEq + Eq + Hash,
      V: Hash + Eq
{
    pub fn new() -> Self {
        DbIndex { map: HashMap::new() }
    }

    pub fn insert(&mut self, key: K, value: V) {
        match self.map.get_mut(&key) {
            None => {
                let mut hs = HashSet::new();
                hs.insert(value);
                self.map.insert(key, hs);
            }
            Some(mut hs) => {
                hs.insert(value);
            }
        }
    }

    pub fn contains(&self, key: &K, value: &V) -> bool {
        match self.map.get(key) {
            None => { false }
            Some(hs) => { hs.contains(value) }
        }
    }

    pub fn get(&self, key: &K) -> Option<&HashSet<V>> {
        self.map.get(key)
    }

    pub fn get_mut(&mut self, key: &K) -> Option<&mut HashSet<V>> {
        self.map.get_mut(key)
    }

    pub fn remove_all_by_key(&mut self, key: &K) -> Option<HashSet<V>> {
        self.map.remove(key)
    }

    pub fn remove_all_by_value(&mut self, value: &V) {
        self.map.iter_mut().for_each(|(_, vs)| {
            vs.remove(value);
        });
    }

    pub fn remove(&mut self, key: &K, value: &V) {
        match self.map.get_mut(key) {
            None => {}
            Some(mut hs) => { hs.remove(value); }
        }
    }

}
