#include "h32127/m32127.h"
QVector<double> m32127::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
