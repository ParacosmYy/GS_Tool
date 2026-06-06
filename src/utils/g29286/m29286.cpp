#include "g29286/m29286.h"
QVector<double> m29286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
