#include "k35470/m35470.h"
QVector<double> m35470::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
