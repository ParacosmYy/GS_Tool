#include "g8526/m8526.h"
QVector<double> m8526::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
