use inotify::Inotify;
use rust_tuple_reactor_serde::FoxTalkSerializable;
use rust_tuple_reactor_serde::tuple::Tuple;
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;

static mut handler: FileWatchHandler = FileWatchHandler::new();

#[no_mangle]
extern "C" static  _foxtalk_ipc_buffer: &mut [u8; 10*1024*1024] = &mut [0; 10*1024*1024];


struct FileWatchHandler{
    handlers: Vec<Inotify>,
    buffer: [u8; 1024*1024]
}
impl FileWatchHandler {
    pub fn new() -> Self {
        FileWatchHandler {
            handlers: Vec::new(),
            buffer: [0; 1024*1024]
        }
    }
    pub fn init(&mut self) {
        let query_tuple = Tuple(vec![
            TupleNoun::Query,
            TupleNoun::Symbol("wants to monitor directory".to_string()),
            TupleNoun::Query,
            TupleNoun::Symbol("for .so files".to_string()),
        ]);
        query_tuple.write_to_buffer(&mut self.buffer, 0);
    }
}

#[no_mangle]
pub extern "C" fn init() {
    unsafe {
        handler.init();
    }
}
#[no_mangle]
pub extern "C" fn free_tuple() {
    // add query tuple to buffer
}
#[no_mangle]
pub extern "C" fn handle() {
    // add query tuple to buffer
}
#[no_mangle]
pub extern "C" fn register_initial_tuples() {
    // add query tuple to buffer
}
#[no_mangle]
pub extern "C" fn teardown() {
    // add query tuple to buffer
}
#[no_mangle]
pub extern "C" fn poll() -> bool {
    false
}
/*                            \
    void init()                                                 \
    void free_tuple()                                                       \
    void handle()                                             \
    void register_initial_tuples()                                               \
    void teardown()
        bool poll()
        inline uint8_t _foxtalk_ipc_buffer[FOXTALK_IPC_BUFFER_SIZE];
 */