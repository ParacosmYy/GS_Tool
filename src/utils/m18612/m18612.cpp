#include "m18612/m18612.h"
QVector<double> m18612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
