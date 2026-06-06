#include "s9278/m9278.h"
QVector<double> m9278::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
