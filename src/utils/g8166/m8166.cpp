#include "g8166/m8166.h"
QVector<double> m8166::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
