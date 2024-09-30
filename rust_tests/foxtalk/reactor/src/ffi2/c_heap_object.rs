/*
 * I have a lot of little bits of data moving back and forth between the reactor's db and
 * C modules. Of primary interest are strings, because (1) they provide for easy debugging, and
 * (2) being able to say that Rust and C strings are equivalent makes things like bootstrapping
 * pretty simple. However, this becomes nightmarish quickly, because I have no idea how to keep
 * track of which ones we need to free and when. I also want the reactor's DB to be the "source
 * of truth" for strings in particular, so that even if a module crashes or unloads or whatever,
 * we don't have a bunch of corrupt strings in the db. Maybe this is a non-issue, since if *any*
 * data about a tuple is corrupt, the whole tuple should be thrown out.
 */
use std::ffi::c_void;
use std::rc::Rc;

#[derive(PartialEq, Eq, Hash, Debug, Clone)]
#[repr(transparent)]
pub struct CHeapObject(Rc<FfiBlob>);

impl CHeapObject {
    // Since we want to be able to reference this in multiple places, we only return ourselves
    // in RC, so we don't preemptively drop our constructor.
    pub fn new(data: *mut c_void, free_fn: unsafe extern "C" fn(*mut c_void)) -> CHeapObject {
        CHeapObject(Rc::new(FfiBlob { data, free_fn }))
    }

    pub fn data(&self) -> *mut c_void {
        self.0.data
    }
}

#[derive(PartialEq, Eq, Hash, Debug)]
struct FfiBlob {
    data: *mut c_void,
    free_fn: unsafe extern "C" fn(*mut c_void)
}

impl Drop for FfiBlob {
    fn drop(&mut self) {
        unsafe { (self.free_fn)(self.data) };
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use std::collections::HashSet;

    extern "C" fn drop_it(_data: *mut c_void) {
        unsafe { *_data.cast() = 4242 };
    }

    #[test]
    fn it_should_drop() {
        let mut n = 0;
        let nptr: *mut usize = &mut n;
        {
            let fi = CHeapObject::new(nptr.cast(), drop_it);
            assert_eq!(n, 0);
        }

        assert_eq!(n, 4242);
    }

    #[test]
    fn it_should_not_drop_from_hashsets() {
        let mut hs1 = HashSet::new();
        let mut hs2 = HashSet::new();

        let mut n = 0;
        let nptr: *mut usize = &mut n;
        {
            let fi = CHeapObject::new(nptr.cast(), drop_it);
            hs1.insert(fi.clone());
            assert_eq!(n, 0);

            hs2.insert(fi.clone());
            assert_eq!(n, 0);

            assert!(hs1.remove(&fi));
            assert_eq!(n, 0);

            assert!(hs2.remove(&fi));
            assert_eq!(n, 0);
        }

        assert_eq!(n, 4242);
    }

    #[test]
    fn it_should_still_be_the_same_data() {
        let mut hs1 = HashSet::new();
        let mut hs2 = HashSet::new();

        let mut n = 0;
        let nptr: *mut usize = &mut n;
        {
            let fi = CHeapObject::new(nptr.cast(), drop_it);
            hs1.insert(fi.clone());
            assert_eq!(n, 0);

            hs2.insert(fi.clone());
            assert_eq!(n, 0);

            hs2.iter().for_each(|f| {
                unsafe { *f.0.data.cast() = 2121; }
            });

            hs1.iter().for_each(|f| {
                assert_eq!(unsafe{ *f.0.data.cast::<usize>() }, 2121);
            });

            assert!(hs1.remove(&fi));
            assert_eq!(n, 2121);

            assert!(hs2.remove(&fi));
            assert_eq!(n, 2121);
        }

        assert_eq!(n, 4242);
    }
}