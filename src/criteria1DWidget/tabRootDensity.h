#ifndef TABROOTDENSITY_H
#define TABROOTDENSITY_H

    #include <QtWidgets>
    #include <QtCharts>

    #include "callout.h"
    #include "criteria1DProject.h"

    class Crit3DMeteoPoint;

    class TabRootDensity : public QWidget
    {
        Q_OBJECT

    public:
        TabRootDensity();
        void computeRootDensity(const Crit1DProject &myProject, int firstYear, int lastYear);

        void on_actionChooseYear(const QString &myYear);
        void updateDate();
        void updateRootDensity();
        void tooltip(bool state, int index, QBarSet *barset);

    private:
        Crit3DCrop m_crop;
        Crit3DMeteoPoint m_meteoPoint;
        std::vector<soil::Crit1DLayer> m_layers;

        unsigned int m_nrLayers;
        int m_year;
        QDate m_lastMeteoDate;

        QComboBox m_yearComboBox;
        QList<double> m_depthLayers;
        QSlider* m_slider;
        QDateEdit *m_currentDateEdit;
        QChartView *chartView;
        QChart *chart;
        QHorizontalBarSeries *seriesRootDensity;
        QBarSet *set;
        QValueAxis *axisX;
        QBarCategoryAxis *axisY;
        QList<QString> categories;
        Callout *m_tooltip;
    };

#endif // TABROOTDENSITY_H
