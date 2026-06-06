#include "a29160/m29160.h"
QVector<double> m29160::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
