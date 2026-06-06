#include "m29612/m29612.h"
QVector<double> m29612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
