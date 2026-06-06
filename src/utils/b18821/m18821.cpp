#include "b18821/m18821.h"
QVector<double> m18821::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
