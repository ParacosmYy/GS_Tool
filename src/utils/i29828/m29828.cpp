#include "i29828/m29828.h"
QVector<double> m29828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
