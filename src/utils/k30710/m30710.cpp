#include "k30710/m30710.h"
QVector<double> m30710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
