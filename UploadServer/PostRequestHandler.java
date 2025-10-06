public class PostRequestHandler implements RequestHandler {
    @Override
    public void handle(HttpServlet servlet, HttpServletRequest req,
                       HttpServletResponse res)
    {
        servlet.doPost(req, res);
    }
}
