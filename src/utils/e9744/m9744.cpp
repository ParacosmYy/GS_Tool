#include "e9744/m9744.h"
QVector<double> m9744::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
