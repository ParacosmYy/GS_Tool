#include "e8144/m8144.h"
QVector<double> m8144::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
