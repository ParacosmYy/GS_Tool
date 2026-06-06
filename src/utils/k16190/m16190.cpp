#include "k16190/m16190.h"
QVector<double> m16190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
