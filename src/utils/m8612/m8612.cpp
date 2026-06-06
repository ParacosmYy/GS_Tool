#include "m8612/m8612.h"
QVector<double> m8612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
