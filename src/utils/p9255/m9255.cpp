#include "p9255/m9255.h"
QVector<double> m9255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
