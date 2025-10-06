public class GetRequestHandler implements RequestHandler {
    @Override
    public void handle(HttpServlet servlet, HttpServletRequest req,
                       HttpServletResponse res)
    {
        servlet.doGet(req, res);
    }
}
