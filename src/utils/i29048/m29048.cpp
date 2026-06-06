#include "i29048/m29048.h"
QVector<double> m29048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
