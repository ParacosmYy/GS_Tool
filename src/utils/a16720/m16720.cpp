#include "a16720/m16720.h"
QVector<double> m16720::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
