#include "n9553/m9553.h"
QVector<double> m9553::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
