#include "d9923/m9923.h"
QVector<double> m9923::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
