#include "g18286/m18286.h"
QVector<double> m18286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
