public interface RequestHandler {
    void handle(HttpServlet servlet, HttpServletRequest req, HttpServletResponse res);
}
