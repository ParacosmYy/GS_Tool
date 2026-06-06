#include "l35591/m35591.h"
QVector<double> m35591::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
