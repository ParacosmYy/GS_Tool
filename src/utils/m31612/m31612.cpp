#include "m31612/m31612.h"
QVector<double> m31612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
