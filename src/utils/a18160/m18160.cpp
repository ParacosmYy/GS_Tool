#include "a18160/m18160.h"
QVector<double> m18160::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
