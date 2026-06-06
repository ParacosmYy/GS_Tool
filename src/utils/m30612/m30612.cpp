#include "m30612/m30612.h"
QVector<double> m30612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
