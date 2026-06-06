#include "m21612/m21612.h"
QVector<double> m21612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
