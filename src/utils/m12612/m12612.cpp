#include "m12612/m12612.h"
QVector<double> m12612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
