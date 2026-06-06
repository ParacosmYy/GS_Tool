#include "c8302/m8302.h"
QVector<double> m8302::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
