#include "m36612/m36612.h"
QVector<double> m36612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
