#include "g29806/m29806.h"
QVector<double> m29806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
