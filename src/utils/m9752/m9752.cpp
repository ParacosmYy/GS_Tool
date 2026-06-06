#include "m9752/m9752.h"
QVector<double> m9752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
