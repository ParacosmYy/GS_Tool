#include "m28612/m28612.h"
QVector<double> m28612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
