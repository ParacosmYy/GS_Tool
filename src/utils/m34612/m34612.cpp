#include "m34612/m34612.h"
QVector<double> m34612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
