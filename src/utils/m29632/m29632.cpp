#include "m29632/m29632.h"
QVector<double> m29632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
