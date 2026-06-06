#include "m19512/m19512.h"
QVector<double> m19512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
