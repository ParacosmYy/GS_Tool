#include "e8744/m8744.h"
QVector<double> m8744::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
