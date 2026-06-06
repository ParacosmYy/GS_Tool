#include "l35211/m35211.h"
QVector<double> m35211::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
