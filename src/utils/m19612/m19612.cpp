#include "m19612/m19612.h"
QVector<double> m19612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
