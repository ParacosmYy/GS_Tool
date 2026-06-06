#include "d9063/m9063.h"
QVector<double> m9063::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
