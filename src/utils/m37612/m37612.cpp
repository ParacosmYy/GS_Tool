#include "m37612/m37612.h"
QVector<double> m37612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
