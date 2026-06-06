#include "l35231/m35231.h"
QVector<double> m35231::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
