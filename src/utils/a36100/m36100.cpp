#include "a36100/m36100.h"
QVector<double> m36100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
