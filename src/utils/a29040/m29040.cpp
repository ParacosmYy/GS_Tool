#include "a29040/m29040.h"
QVector<double> m29040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
