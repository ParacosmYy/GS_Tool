#include "m17612/m17612.h"
QVector<double> m17612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
