#include "g9106/m9106.h"
QVector<double> m9106::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
