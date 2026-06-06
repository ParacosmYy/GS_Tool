#include "i18048/m18048.h"
QVector<double> m18048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
