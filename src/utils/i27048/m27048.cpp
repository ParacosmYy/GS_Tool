#include "i27048/m27048.h"
QVector<double> m27048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
