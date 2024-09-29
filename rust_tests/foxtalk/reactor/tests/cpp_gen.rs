use std::collections::HashSet;
use std::fmt::{Debug, Formatter};
use std::marker::PhantomData;
use anyhow::Debug;

pub struct HasQuery;
pub struct HasHandler;

pub struct CppProgram<T> {
    phantom_data: PhantomData<T>,

    // Required
    get_query_fn_body: Option<String>,
    when_handler_fn_body: Option<String>,

    extra_includes: HashSet<String>,
    extra_code_before_insertion: Option<String>,
    free_tuple_obj_fn_body: Option<String>,
    free_tuple_subj_fn_body: Option<String>,
    extra_code_after_insertion: Option<String>,
}

impl CppProgram<(HasQuery, HasHandler)> {
    ///
    /// Add extra includes to the generated C++ code.
    /// By default, the generated code includes the following:
    ///
    /// ```
    /// string
    /// iostream
    /// vector
    /// ```
    ///
    /// `includes` should be a Vec of Strings where each string will be wrapped in `#include <...>` in the generated code.
    pub fn add_extra_includes(self, includes: Vec<String>) -> Self {
        let new_extra_includes: &mut HashSet<String> = &mut HashSet::from_iter(includes.iter().cloned());

        let extra_includes: HashSet<String> = HashSet::from_iter(self.extra_includes.union(new_extra_includes).into_iter().cloned());
        Self {
            phantom_data: PhantomData,
            get_query_fn_body: self.get_query_fn_body,
            when_handler_fn_body: self.when_handler_fn_body,

            extra_includes,
            extra_code_before_insertion: self.extra_code_before_insertion,
            free_tuple_obj_fn_body: self.free_tuple_obj_fn_body,
            free_tuple_subj_fn_body: self.free_tuple_subj_fn_body,
            extra_code_after_insertion: self.extra_code_after_insertion,
        }
    }

    ///
    /// Add any valid C++ code that will be inserted before the `GetQuery` and `WhenHandler` functions.
    /// Use this to add things like structs that you'll be using, functions you'll want to call, etc
    pub fn add_extra_code_before_insertion(self, code: String) -> Self {
        Self {
            phantom_data: PhantomData,
            get_query_fn_body: self.get_query_fn_body,
            when_handler_fn_body: self.when_handler_fn_body,

            extra_includes: self.extra_includes,
            extra_code_before_insertion: Some(code),
            free_tuple_obj_fn_body: self.free_tuple_obj_fn_body,
            free_tuple_subj_fn_body: self.free_tuple_subj_fn_body,
            extra_code_after_insertion: self.extra_code_after_insertion,
        }
    }

    pub fn add_free_tuple_obj_fn_body(self, body: String) -> Self {
        Self {
            phantom_data: PhantomData,
            get_query_fn_body: self.get_query_fn_body,
            when_handler_fn_body: self.when_handler_fn_body,

            extra_includes: self.extra_includes,
            extra_code_before_insertion: self.extra_code_before_insertion,
            free_tuple_obj_fn_body: Some(body),
            free_tuple_subj_fn_body: self.free_tuple_subj_fn_body,
            extra_code_after_insertion: self.extra_code_after_insertion,
        }
    }

    ///
    /// Add a function body for freeing the subject of the tuple.
    pub fn add_free_tuple_subj_fn_body(self, body: String) -> Self {
        Self {
            phantom_data: PhantomData,
            get_query_fn_body: self.get_query_fn_body,
            when_handler_fn_body: self.when_handler_fn_body,

            extra_includes: self.extra_includes,
            extra_code_before_insertion: self.extra_code_before_insertion,
            free_tuple_obj_fn_body: self.free_tuple_obj_fn_body,
            free_tuple_subj_fn_body: Some(body),
            extra_code_after_insertion: self.extra_code_after_insertion,
        }
    }

    ///
    /// Add any valid C++ code that will be inserted after the `GetQuery` and `WhenHandler` functions.
    pub fn add_extra_code_after_insertion(self, code: String) -> Self {
        Self {
            phantom_data: PhantomData,
            get_query_fn_body: self.get_query_fn_body,
            when_handler_fn_body: self.when_handler_fn_body,

            extra_includes: self.extra_includes,
            extra_code_before_insertion: self.extra_code_before_insertion,
            free_tuple_obj_fn_body: self.free_tuple_obj_fn_body,
            free_tuple_subj_fn_body: self.free_tuple_subj_fn_body,
            extra_code_after_insertion: Some(code),
        }
    }

}

impl Default for CppProgram<()> {
    fn default() -> Self {
        Self {
            phantom_data: PhantomData,
            get_query_fn_body: None,
            when_handler_fn_body: None,
            extra_includes: HashSet::from(["string".to_string(), "iostream".to_string(), "vector".to_string()]),
            extra_code_before_insertion: None,
            free_tuple_obj_fn_body: None,
            free_tuple_subj_fn_body: None,
            extra_code_after_insertion: None
        }
    }
}

impl CppProgram<()> {
    ///
    /// Set the function body for the `GetQuery` function.
    /// The `query` parameter should be valid C++ code when inserted into a function body. It will be
    /// generated like:
    /// ```cpp
    /// Tuple GetQuery() {
    ///   {query}
    /// }
    /// ```
    pub fn set_query(self, query: String) -> CppProgram<HasQuery> {
        CppProgram {
            phantom_data: PhantomData,
            get_query_fn_body: Some(query),
            when_handler_fn_body: self.when_handler_fn_body,
            extra_includes: self.extra_includes,
            extra_code_before_insertion: self.extra_code_before_insertion,
            free_tuple_obj_fn_body: self.free_tuple_obj_fn_body,
            free_tuple_subj_fn_body: self.free_tuple_subj_fn_body,
            extra_code_after_insertion: self.extra_code_after_insertion,
        }
    }

    ///
    /// Set the function body for the `WhenHandler` function.
    /// The `handler` parameter should be valid C++ code when inserted into a function body.
    /// The parameter in the generated function can be used, and has the form `Tuple* result`.
    /// The full generated function looks like
    ///
    /// ```cpp
    /// std::vector<Tuple>* WhenHandler(Tuple* result) {
    ///   {handler}
    /// }
    /// ```
    pub fn set_handler(self, handler: String) -> CppProgram<HasHandler> {
        CppProgram {
            phantom_data: PhantomData,
            get_query_fn_body: self.get_query_fn_body,
            when_handler_fn_body: Some(handler),
            extra_includes: self.extra_includes,
            extra_code_before_insertion: self.extra_code_before_insertion,
            free_tuple_obj_fn_body: self.free_tuple_obj_fn_body,
            free_tuple_subj_fn_body: self.free_tuple_subj_fn_body,
            extra_code_after_insertion: self.extra_code_after_insertion,
        }
    }
}

impl CppProgram<HasQuery> {

    ///
    /// Set the function body for the `WhenHandler` function.
    /// The `handler` parameter should be valid C++ code when inserted into a function body.
    /// The parameter in the generated function can be used, and has the form `Tuple* result`.
    /// The full generated function looks like
    ///
    /// ```cpp
    /// std::vector<Tuple>* WhenHandler(Tuple* result) {
    ///   {handler}
    /// }
    /// ```
    pub fn set_handler(self, handler: String) -> CppProgram<(HasQuery, HasHandler)> {
        CppProgram {
            phantom_data: PhantomData,
            get_query_fn_body: self.get_query_fn_body,
            when_handler_fn_body: Some(handler),
            extra_includes: self.extra_includes,
            extra_code_before_insertion: self.extra_code_before_insertion,
            free_tuple_obj_fn_body: self.free_tuple_obj_fn_body,
            free_tuple_subj_fn_body: self.free_tuple_subj_fn_body,
            extra_code_after_insertion: self.extra_code_after_insertion,
        }
    }
}

impl CppProgram<HasHandler> {
    ///
    /// Set the function body for the `GetQuery` function.
    /// The `query` parameter should be valid C++ code when inserted into a function body. It will be
    /// generated like:
    /// ```cpp
    /// Tuple GetQuery() {
    ///   {query}
    /// }
    /// ```
    pub fn set_query(self, handler: String) -> CppProgram<(HasQuery, HasHandler)> {
        CppProgram {
            phantom_data: PhantomData,
            get_query_fn_body: Some(handler),
            when_handler_fn_body: self.when_handler_fn_body,
            extra_includes: self.extra_includes,
            extra_code_before_insertion: self.extra_code_before_insertion,
            free_tuple_obj_fn_body: self.free_tuple_obj_fn_body,
            free_tuple_subj_fn_body: self.free_tuple_subj_fn_body,
            extra_code_after_insertion: self.extra_code_after_insertion,
        }
    }
}

impl CppProgram<(HasQuery, HasHandler)> {
    pub fn build(self) -> String {
        let include_strs = self
            .extra_includes
            .iter().map(|x| format!("#include <{}>", x))
            .collect::<Vec<String>>().join("\n");
        let before_insertion = self.extra_code_before_insertion.unwrap_or("".to_string());
        let after_insertion = self.extra_code_after_insertion.unwrap_or("".to_string());

        let when_handler_fn_body = self.when_handler_fn_body.unwrap();
        let get_query_fn_body = self.get_query_fn_body.unwrap();

        let subj_free_fn_body = self.free_tuple_subj_fn_body.unwrap_or("".to_string());
        let obj_free_fn_body = self.free_tuple_obj_fn_body.unwrap_or("".to_string());

        let cpp_string = format!(r#"
{include_strs}
extern "C" {{
    struct Tuple {{
        void* subject;
        const char *predicate;
        void* object;
    }};
}}

std::vector<Tuple>* WhenHandler(Tuple* result);
Tuple GetQuery();

extern "C" {{
    void get_query(Tuple* t) {{
        auto q = GetQuery();
        t->subject = q.subject;
        t->predicate = q.predicate;
        t->object = q.object;
    }}

    Tuple* when_handler(Tuple* result, size_t *outLen) {{
        auto res = WhenHandler(result);
        *outLen = res->size();
        return res->data();
    }}
}}

// User-inputted "before insertion" code
{before_insertion}

Tuple GetQuery() {{
    // User-inputted get_query
    {get_query_fn_body}
}}

std::vector<Tuple>* WhenHandler(Tuple* result) {{
    // User-inputted when_handler
    {when_handler_fn_body}
}}

extern "C" void free_tuple_obj(void* obj) {{
    // User-inputted free_tuple_obj
    {obj_free_fn_body}
}}

extern "C" void free_tuple_subj(void* subj) {{
    // User-inputted free_tuple_subj
    {subj_free_fn_body}
}}

// User-inputted "after insertion" code
{after_insertion}
"#
        );
        cpp_string
    }
}

impl Debug for CppProgram<(HasQuery, HasHandler)> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        self.build().fmt(f)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_should_build() {
        let program = CppProgram::default()
            .set_query("query".to_string())
            .set_handler("handler".to_string())
            .build();

        println!("{}", program);
    }
}