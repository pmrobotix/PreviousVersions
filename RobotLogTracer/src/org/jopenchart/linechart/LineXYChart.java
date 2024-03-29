package org.jopenchart.linechart;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.GradientPaint;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Stroke;
import java.util.ArrayList;
import java.util.List;

import org.jopenchart.ArrayOfInt;
import org.jopenchart.Axis;
import org.jopenchart.BottomAxis;
import org.jopenchart.Chart;
import org.jopenchart.DataModel1D;
import org.jopenchart.DataModel2D;
import org.jopenchart.DataModel2DMultiple;
import org.jopenchart.DataModelMultiple;
import org.jopenchart.LeftAxis;
import org.jopenchart.marker.ShapeMarker;

public class LineXYChart extends Chart {

    private Number lowerXRange;
    private Number higherXRange;

    private Number lowerYRange;
    private Number higherYRange;

    private Color fillColor;

    private Double gridXStep;
    private Double gridYStep;

    private Stroke gridStroke;

    private Color gridColor = Color.LIGHT_GRAY;

    private List<Stroke> lineStrokes = new ArrayList<Stroke>();

    private List<ShapeMarker> markers = new ArrayList<ShapeMarker>();

    private Stroke defaultStroke = new BasicStroke(1.6f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND);

    public LineXYChart() {

        left = new LeftAxis();
        bottom = new BottomAxis(true);
        gridStroke = new BasicStroke(1, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND, 0, new float[] { 5, 3 }, 0);

    }

    public Stroke getStroke(int index) {
        if (index >= lineStrokes.size())
            return defaultStroke;
        return lineStrokes.get(index);
    }

    public void setDefaultLineStroke(Stroke s) {
        this.defaultStroke = s;
    }

    public DataModel2DMultiple getDataModel() {
        return (DataModel2DMultiple) this.model;
    }

    public void setDataModel(DataModel2DMultiple m) {
        this.model = m;
    }

    public void setDataModel(DataModel2D m) {
        this.model = new DataModel2DMultiple();
        this.getDataModel().addModel(m);

    }

    public void setLowerRange(Number n) {
        this.lowerXRange = n;
    }

    public void setHigherRange(Number n) {
        this.higherXRange = n;
    }

    public void setFillColor(Color c) {
        this.fillColor = c;
    }

    public Number getLowerXRange() {
        if (this.lowerXRange == null) {
            return this.getDataModel().getMinXValue();
        }
        return this.lowerXRange;

    }

    public Number getHigherXRange() {
        if (this.higherXRange == null) {
            return this.getDataModel().getMaxXValue();
        }
        return this.higherXRange;
    }

    public void setLowerYRange(Number lowerYRange) {
        this.lowerYRange = lowerYRange;
    }

    public Number getLowerYRange() {
        if (this.lowerYRange == null) {
            return this.getDataModel().getMinYValue();
        }
        return this.lowerYRange;

    }

    public void setHigherYRange(Number higherYRange) {
        this.higherYRange = higherYRange;
    }

    public Number getHigherYRange() {
        if (this.higherYRange == null) {
            return this.getDataModel().getMaxYValue();
        }
        return this.higherYRange;
    }

    public void setData(List<Number> x, List<Number> y) {
        DataModel2D m = new DataModel2D();
        m.addAll(x, y);
        setDataModel(m);

    }

    public void setLeftAxis(Axis axis) {
        left = new LeftAxis(axis);
    }

    public void setBottomAxis(Axis axis) {
        bottom = new BottomAxis(axis, true);

    }

    @Override
    public void prepareRendering(Graphics2D g) {

        int leftWidth = 1 + left.getMaxLabelWidth(g) + left.getMarkerLenght() + left.getMarkerSpacing();
        int rightWidth = bottom.getMaxLabelWidth(g) / 2;
        int topHeight = left.getMaxLabelHeight(g) / 2;
        int bottomHeight = 1 + bottom.getMaxLabelHeight(g) + bottom.getMarkerLenght() + bottom.getMarkerSpacing();

        int graphWidth = this.getDimension().width - leftWidth - rightWidth;
        int graphHeight = this.getDimension().height - topHeight - bottomHeight;

        left.setX(0);
        left.setY(topHeight);
        left.setWidth(leftWidth);
        left.setHeight(graphHeight);

        bottom.setX(leftWidth - 1);
        bottom.setY(topHeight + graphHeight);
        bottom.setWidth(graphWidth);
        bottom.setHeight(bottomHeight);

        this.setChartRectangle(new Rectangle(leftWidth, topHeight, graphWidth, graphHeight));

    }

    public void renderPlot(Graphics2D g) {
        renderGrid(g);
        System.out.println("LineXYChart.renderPlot()");
        DataModel2DMultiple models = this.getDataModel();
        for (int index = 0; index < models.getSize(); index++) {

            DataModel2D model1 = models.getModel(index);
            // Chart

            double maxXValue = this.getHigherXRange().doubleValue();
            double maxYValue = this.getHigherYRange().doubleValue();
            double minXValue = this.getLowerXRange().doubleValue();
            double minYValue = this.getLowerYRange().doubleValue();
            double rangeXValue = maxXValue - minXValue;
            double rangeYValue = maxYValue - minYValue;
            int length = model1.getSize();

            int graphPosX = this.getChartRectangle().x;
            int graphPosY = this.getChartRectangle().y;
            int graphWidth = this.getChartRectangle().width;
            int graphHeight = this.getChartRectangle().height;

            double ratiox = (double) graphWidth / rangeXValue;
            double ratioy = (double) graphHeight / rangeYValue;

            ArrayOfInt lx = new ArrayOfInt();
            ArrayOfInt ly = new ArrayOfInt();
            Number n2y = null;
            Number n2x = null;
            for (int i = 0; i < length - 1; i++) {
                Number n1x = model1.getXValueAt(i);
                Number n1y = model1.getYValueAt(i);

                n2x = model1.getXValueAt(i + 1);
                n2y = model1.getYValueAt(i + 1);
                // System.out.println("model:" + i + ":" + n1x + "," + n1y + " -> " + n2x + "," +
                // n2y);

                if (n1y != null) {
                    int x1 = (int) (graphPosX + ((n1x.doubleValue() - minXValue) * ratiox));
                    int y1 = graphPosY + graphHeight - (int) ((n1y.doubleValue() - minYValue) * ratioy);
                    lx.add(x1);
                    ly.add(y1);
                    Color c = g.getColor();
                    g.setColor(Color.DARK_GRAY);
                    if (i == valueIndex && index == modelIndex) {
                        g.setColor(Color.RED);
                    }

                    g.fillRect(x1 - 1, y1 - 1, 3, 3);
                    g.setColor(c);
                } else if (n1y == null && n2y != null && !lx.isEmpty()) {
                    g.drawPolyline(lx.getArray(), ly.getArray(), lx.getSize());
                    lx.clear();
                    ly.clear();
                }

            }
            if (n2y != null) {
                int x1 = (int) (graphPosX + ((n2x.doubleValue() - minXValue) * ratiox));
                int y1 = graphPosY + graphHeight - (int) ((n2y.doubleValue() - minXValue) * ratioy);
                lx.add(x1);
                ly.add(y1);
            }
            // System.out.println("------");
            // for (int i = 0; i < lx.getSize(); i++) {
            // System.out.println(lx.get(i) + "," + ly.get(i));
            // }

            int polyLength = lx.getSize();
            if (fillColor != null && polyLength > 0) {
                g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
                g.setStroke(new BasicStroke());
                g.setColor(fillColor);
                lx.add(lx.get(polyLength - 1));
                ly.add(graphPosY + graphHeight);
                lx.add(lx.get(0));
                ly.add(graphPosY + graphHeight);
                GradientPaint gp = new GradientPaint(graphPosX + graphWidth, graphPosY, Color.white, graphPosX, graphPosY + graphHeight, new Color(230, 235, 250), false);
                g.setPaint(gp);
                g.fillPolygon(lx.getArray(), ly.getArray(), polyLength + 2);

            }
            if (!lx.isEmpty()) {
                g.setPaint(null);
                g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
                g.setStroke(this.getStroke(index));
                g.setColor(this.getColor(index));
                g.drawPolyline(lx.getArray(), ly.getArray(), polyLength);
            }
        }
        // Markers
        renderMarkers(g);

    }

    private void renderMarkers(Graphics2D g) {
        for (ShapeMarker marker : this.markers) {
            marker.draw(this, g);
        }

    }

    private void renderGrid(Graphics2D g) {
        int graphPosX = this.getChartRectangle().x - 1;
        int graphPosY = this.getChartRectangle().y + this.getChartRectangle().height;
        g.setColor(gridColor);
        // Vertical
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

        g.setStroke(gridStroke);

        if (this.gridXStep != null) {
            final double gridXStep2 = (this.gridXStep.doubleValue() * this.getChartRectangle().width) / 100D;

            for (double x = graphPosX + this.getChartRectangle().width; x > graphPosX; x -= gridXStep2) {
                g.drawLine((int) x, graphPosY, (int) x, this.getChartRectangle().y);
            }
        }
        // Horizontal
        if (this.gridYStep != null) {
            final double gridYStep2 = (this.gridYStep.doubleValue() * this.getChartRectangle().height) / 100D;
            for (double y = this.getChartRectangle().y; y < graphPosY; y += gridYStep2) {
                g.drawLine(graphPosX, (int) y, graphPosX + this.getChartRectangle().width, (int) y);
            }
        }
    }

    @Override
    public void renderAxis(Graphics2D g) {
        g.setColor(Color.GRAY);
        g.setStroke(new BasicStroke());
        left.render(g);
        bottom.render(g);
    }

    /**
     * 
     * @param dx 0 - 100
     */
    public void setGridXStep(Double dx) {
        this.gridXStep = dx;

    }

    /**
     * 
     * @param dx 0 - 100
     */
    public void setGridYStep(Double dy) {
        this.gridYStep = dy;

    }

    public void setGridStroke(Stroke stroke) {
        this.gridStroke = stroke;

    }

    public void setGridColor(Color color) {
        this.gridColor = color;

    }

    public void setGridSegment(float lineLength, float blankLenght) {
        this.gridStroke = new BasicStroke(1, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND, 0, new float[] { lineLength, blankLenght }, 0);

    }

    public void setStrokes(List<Stroke> strokes) {
        this.lineStrokes.clear();
        this.lineStrokes.addAll(strokes);

    }

    public void addMarkers(List<ShapeMarker> markers) {
        this.markers.addAll(markers);

    }

    int modelIndex = 0;
    int valueIndex = 0;

    @Override
    public Number highLightAt(int x, int y) {

        DataModel2DMultiple model = this.getDataModel();
        int stop = model.getSize();
        Double nearestX = 0D;
        Double nearestY = 0D;
        double minDist = Double.MAX_VALUE;

        for (int i = 0; i < stop; i++) {
            DataModel2D m = model.getModel(i);
            for (int j = 0; j < m.getSize(); j++) {
                Double xM = m.getXValueAt(j).doubleValue();
                Double yM = m.getYValueAt(j).doubleValue();
                double dx = xM - x;
                double dy = yM - y;
                double dist = dx * dx + dy * dy;
                if (dist < minDist) {
                    modelIndex = i;
                    valueIndex = j;
                    nearestX = xM;
                    nearestY = yM;
                }

            }
        }
        System.out.println("model:" + modelIndex + " index:" + valueIndex);
        return nearestY;
    }
}
