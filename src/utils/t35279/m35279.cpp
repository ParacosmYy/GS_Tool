#include "t35279/m35279.h"
QVector<double> m35279::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
