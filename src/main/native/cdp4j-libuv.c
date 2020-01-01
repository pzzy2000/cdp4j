/**
 * cdp4j Commercial License
 *
 * Copyright 2017, 2020 WebFolder OÜ
 *
 * Permission  is hereby  granted,  to "____" obtaining  a  copy of  this software  and
 * associated  documentation files  (the "Software"), to deal in  the Software  without
 * restriction, including without limitation  the rights  to use, copy, modify,  merge,
 * publish, distribute  and sublicense  of the Software,  and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  IMPLIED,
 * INCLUDING  BUT NOT  LIMITED  TO THE  WARRANTIES  OF  MERCHANTABILITY, FITNESS  FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL  THE AUTHORS  OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "cdp4j-libuv.h"
#include <stdlib.h>

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = (ULONG) suggested_size;
}

static void on_response(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
  if (nread > 0) {
    cdp4j_on_read_callback_java(stream->data, buf->base, (int) nread);
  }
  if (buf->base) {
    free(buf->base);
  }
}

static void on_async_write(uv_write_t* req, int status) {
  if (req) {
    free(req);
  }
}

static void cdp4j_on_process_exit(uv_process_t* process, int64_t exit_status, int term_signal) {
  void* thread = process->data;
  cdp4j_on_process_exit_java(thread);
}

int cdp4j_spawn_process(uv_loop_t* loop, uv_process_t* handle, uv_process_options_t* options) {
  options->exit_cb = cdp4j_on_process_exit;
  return uv_spawn(loop, handle, options);
}

int cdp4j_start_read(uv_pipe_t* out_pipe) {
  return uv_read_start((uv_stream_t*) out_pipe, alloc_buffer, on_response);
}

int cdp4j_write_pipe(uv_loop_t* loop, context_write* context) {
  uv_write_t *request = (uv_write_t*) malloc(sizeof(uv_write_t));
  uv_buf_t buf = uv_buf_init(context->data, context->len);
  return uv_write(request, (uv_stream_t*) context->pipe, &buf, 1, on_async_write);
}

static void on_walk(uv_handle_t *peer, void *arg) {
  if ( ! uv_is_closing(peer) ) {
    uv_close(peer, NULL);
  }
}

void cdp4j_close_loop(uv_loop_t* loop) {
  uv_walk(loop, on_walk, NULL);
  uv_run(loop, UV_RUN_DEFAULT);
}
