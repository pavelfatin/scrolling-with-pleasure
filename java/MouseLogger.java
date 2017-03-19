import java.awt.Dimension;
import java.awt.Point;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionAdapter;
import java.util.DoubleSummaryStatistics;
import java.util.function.Function;
import javax.swing.JFrame;
import javax.swing.SwingUtilities;
import javax.swing.WindowConstants;

public class MouseLogger {
  private static DoubleSummaryStatistics myHStats = new DoubleSummaryStatistics();
  private static DoubleSummaryStatistics myVStats = new DoubleSummaryStatistics();

  private static Point myLastPoint;

  public static void main(String[] args) throws Exception {
    SwingUtilities.invokeLater(() -> {
      JFrame frame = new JFrame("Mouse Logger");

      frame.addMouseListener(new MouseAdapter() {
        @Override
        public void mouseClicked(MouseEvent e) {
          myHStats = new DoubleSummaryStatistics();
          myVStats = new DoubleSummaryStatistics();
          System.out.println("Reset.");
        }
      });

      frame.addMouseMotionListener(new MouseMotionAdapter() {
        @Override
        public void mouseMoved(MouseEvent e) {
          Point p = e.getPoint();

          if (myLastPoint != null) {
            int dx = p.x - myLastPoint.x;
            int dy = p.y - myLastPoint.y;

            if (dx != 0) {
              myHStats.accept(Math.abs(dx));
            }
            if (dy != 0) {
              myVStats.accept(Math.abs(dy));
            }

            System.out.printf("OS\t%+d, %+d\t%s\t%s\n", dx, dy, format(myHStats), format(myVStats));
          }
          myLastPoint = p;
        }
      });

      frame.addMouseWheelListener(e -> System.out.printf(" OS WHEEL %+d (%+.2f)\n",
            e.getWheelRotation(), e.getPreciseWheelRotation()));

      frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
      frame.setPreferredSize(new Dimension(640, 480));
      frame.pack();
      frame.setLocationRelativeTo(null);
      frame.setVisible(true);
    });
  }

  private static String format(DoubleSummaryStatistics stats) {
    int min = stats.getCount() == 0 ? 0 : (int) stats.getMin();
    int max = stats.getCount() == 0 ? 0 : (int) stats.getMax();
    return String.format("% 2d <% 5.2f <% 3d", min, stats.getAverage(), max);
  }
}
